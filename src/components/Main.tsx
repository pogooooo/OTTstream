// components/Main.tsx

import { useEffect, useState, useCallback } from 'react';
import { useNavigate } from 'react-router-dom';
import { useSelector } from 'react-redux';
import styled from 'styled-components';
import type {RootState} from '../Store';
import { fetchVideoList } from '../utils/api'; // 확장자 제거
import type { VideoItem } from '../utils/types';
import UploadModal from './UploadModal'; // UploadModal 임포트

const Main = () => {
    const navigate = useNavigate();
    const isLoggedIn = useSelector((state: RootState) => state.user.isLoggedIn);
    const [videos, setVideos] = useState<VideoItem[]>([]);
    const [isModalOpen, setIsModalOpen] = useState(false); // 모달 상태 추가

    // [수정]: loadVideos를 useCallback으로 감싸 안정화
    const loadVideos = useCallback(async () => {
        const list = await fetchVideoList();
        setVideos(list);
    }, []);

    useEffect(() => {
        if (!isLoggedIn) {
            navigate('/signIn');
        } else {
            // [수정]: 로그인 상태일 때만 목록 로드
            loadVideos();
        }
    }, [isLoggedIn, navigate, loadVideos]); // loadVideos를 의존성 배열에 추가

    if (!isLoggedIn) {
        return <></>;
    }

    return (
        <MainWrapper>
            <h1>메인 스트리밍 화면</h1>
            <VideoGrid>
                {videos.length === 0 ? (
                    <p>현재 업로드된 영상이 없습니다. 영상을 업로드하세요.</p>
                ) : (
                    videos.map(video => (
                        <VideoCard key={video.id}>
                            {/* NOTE: BASE_URL을 config.ts에서 가져와야 하지만, 현재는 하드코딩 유지 */}
                            <Thumbnail src={`http://192.168.35.124:9000/${video.thumbnail}`} alt={video.title}/>
                            <h3>{video.title}</h3>
                            <p>by {video.uploader}</p>
                            <button onClick={() => navigate(`/video/${video.id}`)}>재생</button>
                        </VideoCard>
                    ))
                )}
            </VideoGrid>

            {/* 업로드 버튼 */}
            <UploadButton onClick={() => setIsModalOpen(true)}>+</UploadButton>

            {/* 업로드 모달 */}
            {isModalOpen && (
                <UploadModal
                    isOpen={isModalOpen}
                    onClose={() => setIsModalOpen(false)}
                    onUploadSuccess={loadVideos} // 업로드 성공 시 목록 새로고침
                />
            )}
        </MainWrapper>
    );
};

const MainWrapper = styled.div`
    padding: 20px;
`;



const VideoGrid = styled.div`

display: grid;

grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));

gap: 20px;

`;



const VideoCard = styled.div`

border: 1px solid #ccc;

padding: 10px;

border-radius: 8px;

`;



const Thumbnail = styled.img`

width: 100%;

height: auto;

display: block;

`;

const UploadButton = styled.button`
    position: fixed;
    bottom: 30px;
    right: 30px;
    width: 60px;
    height: 60px;
    border-radius: 50%;
    background-color: #007bff;
    color: white;
    font-size: 30px;
    border: none;
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    cursor: pointer;
    z-index: 1000;
    &:hover {
        background-color: #0056b3;
    }
`;

export default Main;
